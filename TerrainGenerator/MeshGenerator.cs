using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class MeshGenerator : MonoBehaviour
{
    /* Some Settings */
    public bool vegetation = true;

    public int smallGrassFreq = 4;

    /* Spawns */

    public GameObject rockPrefab_0;
    public GameObject rockPrefab_1;
    public GameObject rockPrefab_2;

    public GameObject branchPrefab_0;

    public GameObject grassPrefab_0;
    public GameObject grassPrefab_1;
    public GameObject grassPrefab_2;
    public GameObject grassPrefab_3;
    public GameObject grassPrefab_4;
    public GameObject grassPrefab_5;

    public GameObject[] smallGrasses;

    public GameObject treePrefab;
    public GameObject treePrefab_1;


    /* For Mountain*/

    [Range(0.0f, 10f)]
    public float lowPolyScale = 1f;

    [Range(1f, 10f)]
    public float seaDeep;

    [Range(0.0f, 10f)]
    public float treshold;

    [Range(0.0f, 10f)]
    public float initial_layerDepth;

    [Range(0.0f, 4f)]
    public float layerDepth;

    [Range(0.0f, 1f)]
    public float layerDepth_0;

    [Range(0.0f, 1f)]
    public float layerDepth_1;

    [Range(0.0f, 1f)]
    public float layerDepth_2;

    [Range(0.0f, 1f)]
    public float layerDepth_3;

    [Range(0.0f, 100f)]
    public float depth;

    [Range(0.0f, 100f)]
    public float speed;

    public int xSize = 1;
    public int zSize = 1;

    public int width = 256;
    public int height = 256;

    [Range(0.0f, 10f)]
    public float sandTreshold;

    public float scaleInitial = 20f;
    public float scale0 = 20f;
    public float scale = 20f;
    public float scale1 = 10f;
    public float scale2 = 10f;
    public float scale3 = 50f;

    public float grassScale = 10f;
    [Range(0.0f, 1f)]
    public float grassAmount = 0.5f;

    public float offsetX = 50f;
    public float offsetY = 50f;

    float minTerrainHeight;
    float maxTerrainHeight;

    int[] triangles;
    Vector3[] vertices;
    Vector2[] uvs;
    Color[] colors;
    Vector3[] normals;

    Mesh mesh;
    MeshRenderer renderer;

    List<float> heightList; // heights that is stored for terrain's 3rd dimension
    List<float> perlinHeightList; // heights that is stored for octaves 3rd dimension

    private float vectorVal;

    void Start()
    {
        heightList = new List<float>();
        perlinHeightList = new List<float>();

        mesh = new Mesh();

        /* If you want to have bigger map (more vertices), then you have to need line below */
        //mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;

        GetComponent<MeshFilter>().mesh = mesh;
        renderer = GetComponent<MeshRenderer>();

        CreateShape();
        //StartCoroutine(CreateShape());
    }

    void CreateShape()
    {
        mesh.Clear();

        renderer.material.SetFloat("_Scale", xSize);

        vertices = new Vector3[(xSize + 1) * (zSize + 1)];
        int vertexCount = 0;

        for (int i = 0; i < zSize + 1; i++)
        {
            for (int j = 0; j < xSize + 1; j++)
            {
                float height = CalculateHeight(i, j) * depth;
                heightList.Add(height);
                Vector3 vertex = new Vector3(i * lowPolyScale, height * lowPolyScale, j * lowPolyScale);
                vertices[vertexCount] = vertex;

                if (height > maxTerrainHeight)
                    maxTerrainHeight = height;
                if (height < minTerrainHeight)
                    minTerrainHeight = height;

                vertexCount++;
            }
        }
        mesh.vertices = vertices;

        triangles = new int[xSize * zSize * 6];

        int vert = 0;
        int tris = 0;

        for (int z = 0; z < zSize; z++)
        {
            for (int x = 0; x < xSize; x++)
            {
                triangles[tris + 0] = vert + 1;
                triangles[tris + 1] = vert + xSize + 1;
                triangles[tris + 2] = vert + 0;

                triangles[tris + 3] = vert + xSize + 2;
                triangles[tris + 4] = vert + xSize + 1;
                triangles[tris + 5] = vert + 1;

                vert++;
                tris += 6;
            }
            vert++;
        }
        mesh.triangles = triangles;
        
        uvs = new Vector2[vertices.Length];

        int uvCount = 0;

        for (int i = 0; i < zSize + 1; i++)
        {
            for (int j = 0; j < xSize + 1; j++)
            {
                uvs[uvCount] = new Vector2((float)j / xSize, (float)i / zSize);
                uvCount++;
            }
        }
        mesh.uv = uvs;
        
        mesh.RecalculateNormals();

        normals = mesh.normals;

        colors = new Color[vertices.Length];
        int colorCount = 0;

        for (int i = 0; i < zSize + 1; i++)
        {
            for (int j = 0; j < xSize + 1; j++)
            {
                float slope = Vector3.Angle(normals[colorCount], Vector3.up);
                colors[colorCount] = EvaluateColor(slope, colorCount);


                if(vegetation && slope <= 25f && perlinHeightList[colorCount] > treshold + 0.1f)
                    SpawnNaturalObjects(i, j, heightList[colorCount]);


                colorCount++;
            }
        }
        mesh.colors = colors;

        //offsetX += 0.1f * speed;
        offsetY += 0.1f * speed;

        renderer.material.SetFloat("_Offset", offsetY);
        perlinHeightList.Clear();
        //yield return new WaitForSeconds(0.015f); // ~ 60 fps

        //StartCoroutine(CreateShape());


    }

    /*
    void Update()
    {
        mesh.Clear();
        mesh.vertices = vertices;
        mesh.triangles = triangles;
        mesh.uv = uvs;
        mesh.colors = colors;
        mesh.RecalculateNormals();
    }*/

    void SpawnNaturalObjects(int i, int j, float height)
    {
        /* Spawns */

        /* Grass */

        float x = (float)i + 50f;
        float y = (float)j + 50f;

        float xCoord0 = (float)x / width * grassScale;
        float yCoord0 = (float)y / height * grassScale; // burda buyuk sikinti var

        float perlinHeight = Mathf.PerlinNoise(xCoord0, yCoord0);


        if (perlinHeight <= grassAmount)
        {


            GameObject smallGrass;

            GameObject bigGrass_0;

            if (Random.Range(0f, 1f) <= 1f) // vegetation amount
            {
                /* For Small Vegeatation. For Example common small grass */

                float randomScale = 0;

                int rand = 0;

                if (perlinHeight > grassAmount * 0.8f)
                    rand = 1;
                else
                    rand = 0;

                switch (rand)
                {
                    case 0:

                        bigGrass_0 = Instantiate(grassPrefab_5, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 0.5f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(Random.Range(-5f, 5f), Random.Range(-5f, 5f), Random.Range(-5f, 5f))) as GameObject; // -20, -120, 0
                        randomScale = Random.Range(0.3f, 0.4f);
                        bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);

                        break;
                        /*
                        for (int x = 0; x < smallGrassFreq; x++)
                        {
                            int randGrassIndex = Random.Range(0, smallGrasses.Length);
                            smallGrass = smallGrasses[randGrassIndex];

                            smallGrass = Instantiate(smallGrass, new Vector3((i + Random.Range(-0.15f, 0.15f) + (float)x / 2 + Random.Range(-0.1f, 0.1f)) * lowPolyScale, (height + 0.15f) * lowPolyScale, (j + Random.Range(-0.15f, 0.15f) + (float)x / 2 + Random.Range(-0.1f, 0.1f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), 30 + Random.Range(-5f, 5f), 90 + Random.Range(-5f, 5f))) as GameObject;

                            randomScale = Random.Range(0.8f, 0.8f);
                            smallGrass.transform.localScale = new Vector3(smallGrass.transform.localScale.x * randomScale, smallGrass.transform.localScale.y * randomScale, smallGrass.transform.localScale.z * randomScale);
                        }
                        break;*/
                    case 1:

                        rand = Random.Range(0, 4);

                        switch (rand)
                        {
                            case 0:
                                
                                bigGrass_0 = Instantiate(grassPrefab_2, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 0.7f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), -120 + Random.Range(-5f, 5f), Random.Range(-5f, 5f))) as GameObject;
                                randomScale = Random.Range(1f, 1.5f);
                                bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);
                                
                                break;
                            case 1:
                                bigGrass_0 = Instantiate(grassPrefab_3, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 0.3f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), 30 + Random.Range(-5f, 5f), 90 + Random.Range(-5f, 5f))) as GameObject;
                                randomScale = Random.Range(0.6f, 0.8f);
                                bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);

                                break;
                            case 2:
                                bigGrass_0 = Instantiate(grassPrefab_1, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 1.1f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), -120 + Random.Range(-5f, 5f), Random.Range(-5f, 5f))) as GameObject;
                                randomScale = Random.Range(0.8f, 1.2f);
                                bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);

                                break;
                            case 3:
                                bigGrass_0 = Instantiate(grassPrefab_4, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 1.1f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), -120 + Random.Range(-5f, 5f), Random.Range(-5f, 5f))) as GameObject;
                                randomScale = Random.Range(1.1f, 1.3f);
                                bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);

                                break;/*
                        case 4:
                            bigGrass_0 = Instantiate(grassPrefab_5, new Vector3((i + Random.Range(-0.4f, 0.4f)) * lowPolyScale, (height + 1.1f) * lowPolyScale, (j + Random.Range(-0.4f, 0.4f)) * lowPolyScale), Quaternion.Euler(-20 + Random.Range(-5f, 5f), -120 + Random.Range(-5f, 5f), Random.Range(-5f, 5f))) as GameObject;
                            randomScale = Random.Range(0.3f, 0.4f);
                            bigGrass_0.transform.localScale = new Vector3(bigGrass_0.transform.localScale.x * randomScale, bigGrass_0.transform.localScale.y * randomScale, bigGrass_0.transform.localScale.z * randomScale);

                            break;
                        default:*/
                                      //
                                break;

                        }

                        break;
                        

                }
                
                GameObject tree;

                if (Random.Range(0f, 1f) <= 0.05f && perlinHeight >= grassAmount * 0.95f)
                {
                    if (Random.Range(0f, 1f) <= 0.5)
                        tree = Instantiate(treePrefab, new Vector3(i * lowPolyScale, height * lowPolyScale - 0.1f, j * lowPolyScale), Quaternion.Euler(-90 + Random.Range(-10f, 10f), 0 + Random.Range(-10f, 10f), 0 + Random.Range(-10f, 10f))) as GameObject;
                    else
                        tree = Instantiate(treePrefab_1, new Vector3(i * lowPolyScale, height * lowPolyScale - 0.1f, j * lowPolyScale), Quaternion.Euler(-90 + Random.Range(-10f, 10f), 0 + Random.Range(-10f, 10f), 0 + Random.Range(-10f, 10f))) as GameObject;

                    //float randomScale = Random.Range(0.5f, 1.5f);
                    tree.transform.localScale = new Vector3(tree.transform.localScale.x, tree.transform.localScale.y, tree.transform.localScale.z);
                }

            }
            

        }
        else
        {
            /* Rocks */
            int rand = Random.Range(0, 200);

            GameObject rock;
            float randomScale = 0f;

            switch (rand)
            {
                case 0:
                    rock = Instantiate(rockPrefab_0, new Vector3(i * lowPolyScale, height * lowPolyScale + 0.25f, j * lowPolyScale), Quaternion.Euler(0 + Random.Range(-10f, 10f), 0 + Random.Range(-90f, 90f), 0 + Random.Range(-10f, 10f))) as GameObject;
                    randomScale = Random.Range(0.5f, 1.2f);
                    rock.transform.localScale = new Vector3(rock.transform.localScale.x * randomScale, rock.transform.localScale.y * randomScale, rock.transform.localScale.z * randomScale);
                    break;
                case 1:
                    rock = Instantiate(rockPrefab_1, new Vector3(i * lowPolyScale, height * lowPolyScale, j * lowPolyScale), Quaternion.Euler(Random.Range(-10f, 10f), Random.Range(-90f, 90f), Random.Range(-10f, 10f))) as GameObject;
                    randomScale = Random.Range(0.5f, 1.2f);
                    rock.transform.localScale = new Vector3(rock.transform.localScale.x * randomScale, rock.transform.localScale.y * randomScale, rock.transform.localScale.z * randomScale);
                    break;
                case 2:
                    rock = Instantiate(rockPrefab_2, new Vector3(i * lowPolyScale, height * lowPolyScale, j * lowPolyScale), Quaternion.Euler(Random.Range(-10f, 10f), Random.Range(-90f, 90f), Random.Range(-10f, 10f))) as GameObject;
                    randomScale = Random.Range(0.5f, 1.2f);
                    rock.transform.localScale = new Vector3(rock.transform.localScale.x * randomScale, rock.transform.localScale.y * randomScale, rock.transform.localScale.z * randomScale);
                    break;
                case 3:
                    //
                    break;
                case 4:
                    //
                    break;
                default:
                    //
                    break;

            }

            rand = Random.Range(0, 50);

            switch (rand)
            {
                case 3:
                    rock = Instantiate(branchPrefab_0, new Vector3(i * lowPolyScale, height * lowPolyScale, j * lowPolyScale), Quaternion.Euler(Random.Range(-20f, 20f), Random.Range(-180f, 180f), Random.Range(-20f, 20f))) as GameObject;
                    randomScale = Random.Range(0.3f, 1.5f);
                    rock.transform.localScale = new Vector3(rock.transform.localScale.x * randomScale, rock.transform.localScale.y * randomScale, rock.transform.localScale.z * randomScale);
                    break;
                case 4:
                    //
                    break;
                default:
                    //
                    break;

            }

        }


        /* Spawns */
    }

    Color EvaluateColor(float slope, int colorCount)
    {
        /*
        if (perlinHeightList[colorCount] < sandTreshold)
            return new Color(.5f, 0f, 0f, 1f); // default
            */

        /*
        if (slope <= 18)
        {
            //return new Color(0f, 0f, 0f, 1f); // default
            
            renderer.material.SetFloat("_Color_0", 0f);
            renderer.material.SetFloat("_Color_1", 0f);
            renderer.material.SetFloat("_Color_2", 0f);
            renderer.material.SetFloat("_Color_3", 0f);
            renderer.material.SetFloat("_Color_4", 0f);


        }
        if (slope > 18 && slope <= 25)
        {
            //return new Color(0f, 0f, 1f, 1f); // blue

            renderer.material.SetFloat("_Color_0", 0f);
            renderer.material.SetFloat("_Color_1", 0f);
            renderer.material.SetFloat("_Color_2", 1f);
            renderer.material.SetFloat("_Color_3", 0f);
            renderer.material.SetFloat("_Color_4", 0f);
        }
        else if (slope > 25 && slope <= 35)
        {
            //return new Color(0f, 1f, 0f, 1f); // green channel

            renderer.material.SetFloat("_Color_0", 0f);
            renderer.material.SetFloat("_Color_1", 1f);
            renderer.material.SetFloat("_Color_2", 0f);
            renderer.material.SetFloat("_Color_3", 0f);
            renderer.material.SetFloat("_Color_4", 0f);
        }
        else
        {
            //return new Color(1f, 0f, 0f, 1f); // red channel

            renderer.material.SetFloat("_Color_0", 1f);
            renderer.material.SetFloat("_Color_1", 0f);
            renderer.material.SetFloat("_Color_2", 0f);
            renderer.material.SetFloat("_Color_3", 0f);
            renderer.material.SetFloat("_Color_4", 0f);
        }*/

        
        if (perlinHeightList[colorCount] < sandTreshold)
            return new Color(0f, 0f, 0f, 1f); // magenta channel
        if (slope <= 18)
            return new Color(0f, 0f, 0f, 0f); // default
        if (slope > 18 && slope <= 25)
            return new Color(0f, 0f, 1f, 0f); // blue
        else if (slope > 25 && slope <= 35)
            return new Color(0f, 1f, 0f, 0f); // green channel
        else
            return new Color(1f, 0f, 0f, 0f); // red channel
            
    }

    float CalculateHeight(int i, int j)
    {
        float x = (float)i + offsetX;
        float y = (float)j + offsetY;

        /* Main Curve */
        float xCoordA = (float)x / width * scaleInitial;
        float yCoordA = (float)y / height * scaleInitial;

        /* Main Curve */
        float xCoord0 = (float)x / width * scale0;
        float yCoord0 = (float)y / height * scale0;

        /* Main Curve */
        float xCoord = (float)x / width * scale;
        float yCoord = (float)y / height * scale;

        /* Second Curve */
        float xCoord1 = (float)x / width * scale1;
        float yCoord1 = (float)y / height * scale1;

        /* Third Curve */
        float xCoord2 = (float)x / width * scale2;
        float yCoord2 = (float)y / height * scale2;

        /* Forth Curve */
        float xCoord3 = (float)x / width * scale3;
        float yCoord3 = (float)y / height * scale3;

        float height5 = Mathf.PerlinNoise(xCoordA, yCoordA) * initial_layerDepth;
        float height4 = Mathf.PerlinNoise(xCoord0, yCoord0) * layerDepth;
        float height0 = Mathf.PerlinNoise(xCoord, yCoord) * layerDepth_0;
        float height1 = Mathf.PerlinNoise(xCoord1, yCoord1) * layerDepth_1;
        float height2 = Mathf.PerlinNoise(xCoord2, yCoord2) * layerDepth_2;
        float height3 = Mathf.PerlinNoise(xCoord3, yCoord3) * layerDepth_3;

        float totalheight = height0 + height1 + height2 + height3 + height4 + height5;
        perlinHeightList.Add(totalheight);

        if (totalheight < treshold)
             return treshold - ((treshold - totalheight) / seaDeep);
            //return treshold;
        else

            return totalheight;
    }
}
